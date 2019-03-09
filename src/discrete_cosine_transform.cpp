/* This file is a part of "Steganography" a C++ steganography tool.

Copyright (C) 2019 James Lee <jamesl33info@gmail.com>.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "discrete_cosine_transform.hpp"

/**
 * The available DCT coefficient swap pairs.
 */
const std::vector<std::tuple<std::tuple<int, int>, std::tuple<int, int>>> coefficients = {{{0, 2}, {2, 0}},
                                                                                          {{1, 2}, {2, 1}},
                                                                                          {{0, 3}, {3, 0}},
                                                                                          {{1, 3}, {3, 1}}};

/**
 * Encode the payload file into the carrier image by swapping DCT coefficients.
 *
 * @param payload_path Path to the file we are encoding.
 * @exception EncodeException thrown when encoding fails.
 */
void DiscreteCosineTransform::Encode(const boost::filesystem::path &payload_path)
{
    // Ensure that the carrier has enough room for the payload
    if (boost::filesystem::file_size(payload_path) * 8 > this->image_capacity)
    {
        throw EncodeException("Error: Failed to encode payload, carrier too small");
    }

    // Convert the filename to a vector<unsigned char>
    std::string payload_filename = payload_path.filename().string();
    std::vector<unsigned char> filename_bytes(payload_filename.begin(), payload_filename.end());

    // Encode the filename into the carrier image
    this->EncodeChunkLength(0, filename_bytes.size());
    this->EncodeChunk(32, filename_bytes);

    // Read the payload into a vector<unsigned char>
    std::vector<unsigned char> payload_bytes = this->ReadPayload(payload_path);

    this->EncodeChunkLength(32 + filename_bytes.size() * 8, payload_bytes.size());
    this->EncodeChunk(64 + filename_bytes.size() * 8, payload_bytes);

    // Encode the payload into the carrier image
    boost::filesystem::path steg_image_filename = this->image_path.filename();
    steg_image_filename.replace_extension(".jpg");

    // Write the steganographic image
    cv::imwrite("steg-" + steg_image_filename.string(), this->image, std::vector<int>{CV_IMWRITE_JPEG_QUALITY, 100});
}

/**
 * Decode the payload from the steganographic image by comparing DCT
 * coefficients.
 */
void DiscreteCosineTransform::Decode()
{
    // Decode the filename from the steganographic image
    unsigned int filename_length = this->DecodeChunkLength(0);
    std::vector<unsigned char> filename_bytes = this->DecodeChunk(32, 32 + filename_length * 8);

    // Convert the filename vector<unsigned char> to a string
    std::string payload_filename(filename_bytes.begin(), filename_bytes.end());

    // Decode the payload from the steganographic image
    unsigned int payload_length = this->DecodeChunkLength(32 + filename_length * 8);
    std::vector<unsigned char> payload_bytes = this->DecodeChunk(64 + filename_length * 8, 64 + filename_length * 8 + payload_length * 8);

    // Write the decoded payload
    this->WritePayload("steg-" + payload_filename, payload_bytes);
}

/**
 * Encode a chunk of information into the carrier image.
 *
 * Before encoding a chunk of information you "should" first encode its length
 * using the EncodeChunkLength function.
 *
 * @param start The bit index to start encoding at.
 * @param chunk The chunk of information which will be encoded.
 */
void DiscreteCosineTransform::EncodeChunk(const int &start, const std::vector<unsigned char> &chunk)
{
    // Convert the carrier image from unsigned char to floating point
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    int bits_written = 0;

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform the forward dct
            cv::dct(block, trans);

            // Swap N DCT coefficients
            for (int i = 0; i < this->swap_count && i < coefficients.size(); i++)
            {
                std::tuple<int, int> a = std::get<0>(coefficients[i]);
                std::tuple<int, int> b = std::get<1>(coefficients[i]);

                // Embed the current chunk bit in the carrier
                this->SwapCoefficients(&trans, this->GetBit(chunk[bits_written / 8], bits_written % 8), a, b);

                if (++bits_written == chunk.size() * 8)
                {
                    break;
                }
            }

            // Perform the inverse dct
            cv::idct(trans, block);

            // Recentre the coefficients around 128
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) += 128;
                }
            }

            // We have finished embedding, clean up
            if (bits_written == chunk.size() * 8)
            {
                // Merge the channels into a single image
                cv::Mat mergedfp;
                cv::merge(channels, mergedfp);

                // Convert the carrier image to from floating point to unsigned char
                mergedfp.convertTo(this->image, CV_8U);
                return;
            }
        }
    }
}

/**
 * Encode a 32bit integer stating the length of the following chunk into the
 * carrier image.
 *
 * @param start The bit index to start encoding at.
 * @param chunk_length The length of the next chunk in bytes.
 */
void DiscreteCosineTransform::EncodeChunkLength(const int &start, const unsigned int &chunk_length)
{
    // Convert the carrier image from unsigned char to floating point
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    int bits_written = 0;

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform the forward dct
            cv::dct(block, trans);

            // Swap N DCT coefficients
            for (int i = 0; i < this->swap_count && i < coefficients.size(); i++)
            {
                std::tuple<int, int> a = std::get<0>(coefficients[i]);
                std::tuple<int, int> b = std::get<1>(coefficients[i]);

                this->SwapCoefficients(&trans, this->GetBit(chunk_length, bits_written), a, b);

                if (++bits_written == 32)
                {
                    break;
                }
            }

            // Perform the inverse dct
            cv::idct(trans, block);

            // Recentre the coefficients around 128
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) += 128;
                }
            }

            // We have finished embedding, clean up
            if (bits_written == 32)
            {
                cv::Mat mergedfp;
                cv::merge(channels, mergedfp);

                mergedfp.convertTo(this->image, CV_8U);
                return;
            }
        }
    }
}

/**
 * Attempt to decode a chunk of information from the steganographic image.
 *
 * @param start The bit index to start decoding at.
 * @param end The bit index to stop decoding at.
 * @return The chunk of information read from the steganographic image.
 * @exception DecodeException Thrown when decoding fails.
 */
std::vector<unsigned char> DiscreteCosineTransform::DecodeChunk(const int &start, const int &end)
{
    std::vector<unsigned char> chunk_bytes((end - start) / 8);

    // Convert the steganographic image from unsigned char to floating point
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    int bits_read = 0;

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct
            cv::dct(block, trans);

            // Read from N swapped DCT coefficients
            for (int i = 0; i < this->swap_count && i < coefficients.size(); i++)
            {
                std::tuple<int, int> a = std::get<0>(coefficients[i]);
                std::tuple<int, int> b = std::get<1>(coefficients[i]);

                this->SetBit(&chunk_bytes[bits_read / 8], bits_read % 8,
                        (trans.at<float>(std::get<0>(a), std::get<1>(a)) < trans.at<float>(std::get<0>(b), std::get<1>(b))));

                if (++bits_read == end - start)
                {
                    // We have finished decoding, return the chunk
                    return chunk_bytes;
                }
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload");
}

/**
 * Attempt to decode the 32bit integer stating the length of the following
 * chunk.
 *
 * @param start The bit index to start decoding at.
 * @return The length of the following chunk.
 * @exception DecodeException Thrown when decoding fails.
 */
unsigned int DiscreteCosineTransform::DecodeChunkLength(const int &start)
{
    unsigned int chunk_length = 0;

    // Convert the steganographic image from unsigned char to floating point
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    int bits_read = 0;

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct
            cv::dct(block, trans);

            // Read from N swapped DCT coefficients
            for (int i = 0; i < this->swap_count && i < coefficients.size(); i++)
            {
                std::tuple<int, int> a = std::get<0>(coefficients[i]);
                std::tuple<int, int> b = std::get<1>(coefficients[i]);

                this->SetBit(&chunk_length, bits_read,
                        (trans.at<float>(std::get<0>(a), std::get<1>(a)) < trans.at<float>(std::get<0>(b), std::get<1>(b))));

                if (++bits_read == 32)
                {
                    // We have decoded the integer, check if it's valid
                    if (chunk_length >= this->image_capacity || chunk_length == 0)
                    {
                        throw DecodeException("Error: Failed to decode payload length");
                    }

                    return chunk_length;
                }
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload length");
}

/**
 * Swap two DCT coefficients.
 *
 * Swap two DCT coefficients and apply a persistence value to ensure that the
 * data survives the compression process.
 *
 * @param block A pointer to the block which is currently be operated on.
 * @param value The value which is being stored, will be 0 or 1.
 */
void DiscreteCosineTransform::SwapCoefficients(cv::Mat *block, const int &value, const std::tuple<int, int> &a, const std::tuple<int, int> &b)
{
    // Read two coefficients from the image block
    float low = block->at<float>(std::get<0>(a), std::get<1>(a));
    float high = block->at<float>(std::get<0>(b), std::get<1>(b));

    // Swap the coefficients so that low is low and high is high
    if (value && (low > high))
    {
        std::swap(low, high);
    }
    else if (!value && (low < high))
    {
        std::swap(low, high);
    }

    // Apply the persistence value
    if (value && (low == high || low < high))
    {
        low -= this->persistence;
        high += this->persistence;
    }
    else if (!value && (low == high || low > high))
    {
        low += this->persistence;
        high -= this->persistence;
    }

    // Write the coefficients back to the block
    block->at<float>(std::get<0>(a), std::get<1>(a)) = low;
    block->at<float>(std::get<0>(b), std::get<1>(b)) = high;
}
